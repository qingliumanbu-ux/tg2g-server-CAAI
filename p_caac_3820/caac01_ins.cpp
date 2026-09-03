/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      179297
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 成本中心信息修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


//外部函数声明

/*<remark>=========================================================
/// <summary>
/// 成本中心新增
/// <para>
/// 1.根据传入的前台数据，新增数据进tcaac01表。
/// 2.写物料异动履历
/// </para>
/// <para>数据库表：TCAAC01(成本中心信息表)         </para>
/// <para>主调用函数：前台FormCAAC01画面(F3)调用。   </para>
/// </summary>
/// <param name="TCAAC01">整表数据    </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(caac01_ins)

int f_caac01_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
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
	CModel tcaac01("TCAAC01");

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
			tcaac01.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC01类型的对象
			tcaac01.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			if (tcaac01["COST_CENTER"].ToString().Trim() == "")
			{
				sprintf(s.msg,"成本中心无值!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if(tcaac01.QueryCount("COST_CENTER,DEPT_CODE")!=0)
			{
				sprintf(s.msg,"成本中心[%s]在成本中心表中存在!",(const char*)tcaac01["COST_CENTER"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			
			tcaac01["REC_CREATOR"]=s.userid;   //记录创建责任者
			tcaac01["REC_CREATE_TIME"]=datetime;   //记录创建时刻
			tcaac01["VALID_FLAG"]="1";   //生效标志

            //新增一条记录
            tcaac01.Insert();
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
