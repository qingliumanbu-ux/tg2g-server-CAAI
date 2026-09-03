/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhoabin
Version:    1.0
Date:     2020-07-20
Description: 加工费修改
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caac16_upd)

int f_caac16_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;


	/* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac16("TCAAC16");

	// 数据库SQL操作字符串
	CString  sql("");


	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		for (int i = 0; i < count; i++)
		{
			tcaac16.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			//修改信息
			tcaac16["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaac16["REC_REVISE_TIME"] = datetime;   //记录修改时刻

			//修改一条记录
			tcaac16.Update("PRICE_UNIT,TAX_RATE_NAME,TAX_RATE,FIX_LEN_FROM,FIX_LEN_TO,ACCU_GRADE,REMARK,OUT_TASK_DESC,OUT_UNIT_NAME","OUT_TASK_NO,PROC_MODE,OUT_UNIT_CODE,RECV_DEPT,THICK_MAX,THICK_MIN,EFFECT_DATE");
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sql + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}