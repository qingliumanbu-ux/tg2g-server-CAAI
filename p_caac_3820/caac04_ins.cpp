/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhl
Version:    1.0
Date:     2016-03-10
Description: 产副品代码新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac04_ins)

int f_caac04_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
	int   outBlockRow=0;
	int   ren = 0; 
    int   v_count=0;
	int   blkNum = 0;
	CDecimal v_roll_wt=0;
	CDecimal v_roll_num=0;
	CDecimal v_bundle_wt=0;
	CDecimal v_out_mat_num=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac11("TCAAC11");

	// 数据库SQL操作字符串
	CString  sql("");

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac11.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC11类型的对象
			tcaac11.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if(tcaac11["MAT_CODE"].ToString().Trim() == "")
			{
				sprintf(s.msg,"产副品代码无值!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac11["MAT_NAME"].ToString().Trim() == "")
			{
				sprintf(s.msg,"产副品名称无值!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac11.QueryCount("MAT_CODE")!=0)
			{
				sprintf(s.msg,"产副品代码[%s]在产副品代码信息表中已存在!",(const char*)tcaac11["MAT_CODE"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			
			tcaac11["REC_CREATOR"]=s.userid;   //记录创建责任者
			tcaac11["REC_CREATE_TIME"]=datetime;   //记录创建时刻
			tcaac11["VALID_FLAG"]="1";   //生效标志

            //新增一条记录
            tcaac11.Insert();
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,"数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1,1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
