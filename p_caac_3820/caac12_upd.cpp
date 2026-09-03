/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      179297
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 物料价格修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//外部函数声明

/*<remark>=========================================================
/// <summary>
/// 物料价格修改
/// <para>
/// 1.根据传入的物料价格，修改tcaac12表数据。
/// </para>
/// <para>数据库表：TCAAC12(物料价格表)         </para>
/// <para>主调用函数：前台FormCAAC12画面(F4)调用。   </para>
/// </summary>
/// <param name="TCAAC12">整表数据    </param>
/// <returns>处理结果</returns>
===========================================================</remark>*/

// service入口
BM2F_ENTERACE(caac12_upd)

int f_caac12_upd(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
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
	CDecimal  v_total_count = 0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
    CModel tcaac12("TCAAC12");

	// 数据库SQL操作字符串
	CString  sql("");
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);


    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
        //获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();
		
		for (int i = 0; i <  count ; i++ )
		{
            //重置头文件,获得头文件中的默认值
			tcaac12.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据
			tcaac12.MergeFrom(bcls_rec->Tables[0].Rows[i]);
		
			if(tcaac12["PRICE_UNIT"].ToDecimal() == 0)
			{
				sprintf(s.msg,"价格为0!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			sqlstr = " select mat_name from tcaac11 "
				" where mat_code=@mat_code ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("mat_code", tcaac12["MAT_CODE"]);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaac12["MAT_NAME"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
			//修改信息
			tcaac12["REC_REVISOR"]=s.userid;   //记录修改责任者
			tcaac12["REC_REVISE_TIME"]=datetime;   //记录修改时刻

            //修改一条记录
            tcaac12.Update("REC_REVISOR,REC_REVISE_TIME,PRICE_UNIT,MAT_NAME,BACK_CODE_1,UNIT","MAT_CODE,VALID_TIME_START,PRICE_TERMS");
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
